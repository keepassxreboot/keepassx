/*
 *  Copyright (C) 2017 KeePassXC Team <team@keepassxc.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Conforms to RFC 4648. For details, see: https://tools.ietf.org/html/rfc4648

#include "base32.h"

constexpr quint64 MASK_40BIT = quint64(0xF8) << 32;
constexpr quint64 MASK_35BIT = quint64(0x7C0000000);
constexpr quint64 MASK_25BIT = quint64(0x1F00000);
constexpr quint64 MASK_20BIT = quint64(0xF8000);
constexpr quint64 MASK_10BIT = quint64(0x3E0);

constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
constexpr quint8 ALPH_POS_2 = 26;

constexpr quint8 ASCII_2 = static_cast<quint8>('2');
constexpr quint8 ASCII_7 = static_cast<quint8>('7');
constexpr quint8 ASCII_A = static_cast<quint8>('A');
constexpr quint8 ASCII_Z = static_cast<quint8>('Z');
constexpr quint8 ASCII_a = static_cast<quint8>('a');
constexpr quint8 ASCII_z = static_cast<quint8>('z');
constexpr quint8 ASCII_EQ = static_cast<quint8>('=');

Optional<QByteArray> Base32::decode(const QByteArray& encodedData)
{
    if (encodedData.size() <= 0)
        return Optional<QByteArray>("");

    if (encodedData.size() % 8 != 0)
        return Optional<QByteArray>();

    int nPads = 0;
    for (int i = -1; i > -7; --i) {
        if ('=' == encodedData[encodedData.size()+i])
            ++nPads;
    }

    int specialOffset;
    int nSpecialBytes;

    switch (nPads) { // in {0, 1, 3, 4, 6}
    case 1:
        nSpecialBytes = 4;
        specialOffset = 3;
        break;
    case 3:
        nSpecialBytes = 3;
        specialOffset = 1;
        break;
    case 4:
        nSpecialBytes = 2;
        specialOffset = 4;
        break;
    case 6:
        nSpecialBytes = 1;
        specialOffset = 2;
        break;
    default:
        nSpecialBytes = 0;
        specialOffset = 0;
    }


    Q_ASSERT(encodedData.size() > 0);
    const int nQuantums = encodedData.size() / 8;
    const int nBytes = (nQuantums - 1) * 5 + nSpecialBytes;

    QByteArray data(nBytes, Qt::Uninitialized);

    int i = 0;
    int o = 0;

    while (i < encodedData.size()) {
        quint64 quantum = 0;
        int nQuantumBytes = 5;

        for (int n = 0; n < 8; n++) {
            quint8 ch = static_cast<quint8>(encodedData[i++]);
            if ((ASCII_A <= ch && ch <= ASCII_Z) || (ASCII_a <= ch && ch <= ASCII_z)) {
                ch -= ASCII_A;
                if (ch >= ALPH_POS_2)
                    ch -= ASCII_a - ASCII_A;
            } else {
                if (ch >= ASCII_2 && ch <= ASCII_7) {
                    ch -= ASCII_2;
                    ch += ALPH_POS_2;
                } else {
                    if (ASCII_EQ == ch) {
                        if(i == encodedData.size()) {
                            // finished with special quantum
                            quantum >>= specialOffset;
                            nQuantumBytes = nSpecialBytes;
                        }
                        continue;
                    } else {
                        // illegal character
                        return Optional<QByteArray>();
                    }
                }
            }

            quantum <<= 5;
            quantum |= ch;
        }

        const int offset = (nQuantumBytes - 1) * 8;
        quint64 mask = quint64(0xFF) << offset;
        for (int n = offset; n >= 0; n -= 8) {
            char c = static_cast<char>((quantum & mask) >> n);
            data[o++] = c;
            mask >>= 8;
        }
    }

    return Optional<QByteArray>(data);
}

QByteArray Base32::encode(const QByteArray& data)
{
    if (data.size() < 1)
        return QByteArray();

    const int nBits = data.size() * 8;
    const int rBits = nBits % 40; // in {0, 8, 16, 24, 32}
    const int nQuantums = nBits / 40 + (rBits > 0 ? 1 : 0);
    QByteArray encodedData(nQuantums * 8, Qt::Uninitialized);
    
    int i = 0;
    int o = 0;
    int n;
    quint64 mask;
    quint64 quantum;

    // 40-bits of input per input group
    while (i + 5 <= data.size()) {
        quantum = 0;
        for (n = 32; n >= 0; n -= 8) {
            quantum |= (static_cast<quint64>(data[i++]) << n);
        }

        mask = MASK_40BIT;
        int index;
        for (n = 35; n >= 0; n -= 5) {
            index = (quantum & mask) >> n;
            encodedData[o++] = alphabet[index];
            mask >>= 5;
        }
    }

    // < 40-bits of input at final input group
    if (i < data.size()) {
        Q_ASSERT(rBits > 0);
        quantum = 0;
        for (n = rBits - 8; n >= 0; n -= 8)
            quantum |= static_cast<quint64>(data[i++]) << n;

        switch (rBits) {
        case 8:  // expand to 10 bits
            quantum <<= 2;
            mask = MASK_10BIT;
            n = 5;
            break;
        case 16: // expand to 20 bits
            quantum <<= 4;
            mask = MASK_20BIT;
            n = 15;
            break;
        case 24: // expand to 25 bits
            quantum <<= 1;
            mask = MASK_25BIT;
            n = 20;
            break;
        default: // expand to 35 bits
            Q_ASSERT(rBits == 32);
            quantum <<= 3;
            mask = MASK_35BIT;
            n = 30;
        }

        while (n >= 0) {
            int index = (quantum & mask) >> n;
            encodedData[o++] = alphabet[index];
            mask >>= 5;
            n -= 5;
        }

        // add pad characters
        while (o < encodedData.size())
            encodedData[o++] = '=';
    }

    Q_ASSERT(encodedData.size() == o);
    return encodedData;
}

