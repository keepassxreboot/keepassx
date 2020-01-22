/*
 *  Copyright (C) 2020 KeePassXC Team <team@keepassxc.org>
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

#include "EditMarkupWidget.h"

#include "gui/GuiTools.h"

EditMarkupWidget::EditMarkupWidget(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::EditMarkupWidget)
{
    m_ui->setupUi(this);
    connect(m_ui->sourceEdit, SIGNAL(textChanged()), SLOT(handleTextChange()));
    connect(m_ui->tabWidget, SIGNAL(currentChanged(int)), SLOT(updatePreview()));
}

void EditMarkupWidget::setReadOnly(bool readOnly)
{
    m_ui->sourceEdit->setReadOnly(readOnly);
}

void EditMarkupWidget::setMarkup(const QString& markup)
{
    m_ui->sourceEdit->setPlainText(markup);
    updatePreview();
}

QString EditMarkupWidget::getMarkup()
{
    return m_ui->sourceEdit->toPlainText();
}

void EditMarkupWidget::clear()
{
    m_ui->sourceEdit->clear();
    updatePreview();
}

void EditMarkupWidget::handleTextChange()
{
    updatePreview();
    emit textChanged();
}

void EditMarkupWidget::updatePreview()
{
    QTextDocument* doc = new QTextDocument(m_ui->previewEdit);
    GuiTools::buildDocumentFromMarkup(doc, getMarkup());
    m_ui->previewEdit->setDocument(doc);
}
