/*
*  Copyright (C) 2013 Francois Ferrand
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

#ifndef KEYACCEPTDIALOG_H
#define KEYACCEPTDIALOG_H

#include <QDialog>
#include <QScopedPointer>

namespace Ui {
class KeyAcceptDialog;
}

class KeyAcceptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeyAcceptDialog(QWidget *parent = nullptr);
    ~KeyAcceptDialog();

    void setItems(const QList<QString> & items);
    void setItemChecked(int itemIndex, bool checked);
    void setItemEnabled(int itemIndex, bool enabled);
    QList<int> getCheckedItems();

    QString getKeyName();

public slots:
    void keyEditChanged(const QString &text);

private:
    QScopedPointer<Ui::KeyAcceptDialog> ui;
};

#endif // KEYACCEPTDIALOG_H

