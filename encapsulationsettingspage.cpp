#include "encapsulationsettingspage.h"
#include "ui_encapsulationsettingswidget.h"
#include "encapsulationconstants.h"

#include <cpptools/cpptoolsconstants.h>
#include <coreplugin/icore.h>

#include <QCoreApplication>
#include <QSettings>

#include <QTextStream>

namespace Encapsulation
{
namespace Internal
{

void Settings::toSettings(QSettings *s) const
{
    s->beginGroup(QString::fromLatin1(Constants::SETTINGS_GROUP));
    s->setValue(QString::fromLatin1(Constants::SETTING_PREFIX), fieldPrefix);
    s->setValue(QString::fromLatin1(Constants::SETTING_SUFFIX), fieldSuffix);
    s->setValue(QString::fromLatin1(Constants::SETTING_GETSTRING), addGetString);
    s->setValue(QString::fromLatin1(Constants::SETTING_CCASE), useCamelCase);
    s->setValue(QString::fromLatin1(Constants::SETTING_MUTFIRST), mutatorFirst);
    s->setValue(QString::fromLatin1(Constants::SETTING_CPPFILE), cppFile);
	s->endGroup();
}

void Settings::fromSettings(QSettings *s)
{
    s->beginGroup(QString::fromLatin1(Constants::SETTINGS_GROUP));
    fieldPrefix = s->value(QString::fromLatin1(Constants::SETTING_PREFIX), QString::fromLatin1("m_")).toString();
    fieldSuffix = s->value(QString::fromLatin1(Constants::SETTING_SUFFIX)).toString();
    addGetString = s->value(QString::fromLatin1(Constants::SETTING_GETSTRING), false).toBool();
    useCamelCase = s->value(QString::fromLatin1(Constants::SETTING_CCASE), true).toBool();
    mutatorFirst = s->value(QString::fromLatin1(Constants::SETTING_MUTFIRST), true).toBool();
    cppFile = s->value(QString::fromLatin1(Constants::SETTING_CPPFILE), false).toBool();
	s->endGroup();
}

bool Settings::equals(const Settings &rhs) const
{
	return rhs.addGetString == addGetString &&
		   rhs.fieldPrefix == fieldPrefix &&
		   rhs.fieldSuffix == fieldSuffix &&
		   rhs.mutatorFirst == mutatorFirst &&
		   rhs.useCamelCase == useCamelCase &&
		   rhs.cppFile == cppFile;
}

SettingsWidget::SettingsWidget(QWidget *parent):
	QWidget(parent),
	m_ui(new Ui::SettingsWidget)
{
	m_ui->setupUi(this);
}

Settings SettingsWidget::settings() const
{
	Settings s;
	s.fieldPrefix = m_ui->prefixEdit->text();
	s.fieldSuffix = m_ui->suffixEdit->text();
	s.addGetString = m_ui->addGetCheckBox->isChecked();
	s.mutatorFirst = m_ui->mutatorFirstCheckBox->isChecked();
	s.useCamelCase = m_ui->camelCaseCheckBox->isChecked();
	s.cppFile = m_ui->cppFileCheckBox->isChecked();
	return s;
}

void SettingsWidget::setSettings(const Settings &s)
{
	m_ui->prefixEdit->setText(s.fieldPrefix);
	m_ui->suffixEdit->setText(s.fieldSuffix);
	m_ui->addGetCheckBox->setChecked(s.addGetString);
	m_ui->mutatorFirstCheckBox->setChecked(s.mutatorFirst);
	m_ui->camelCaseCheckBox->setChecked(s.useCamelCase);
	m_ui->cppFileCheckBox->setChecked(s.cppFile);
}

QString SettingsWidget::searchKeywords() const
{
	QString keywords;

	QTextStream(&keywords)
	<< m_ui->addGetCheckBox->text() << ' '
	<< m_ui->behaviorGroupBox->windowTitle() << ' '
	<< m_ui->camelCaseCheckBox->text() << ' '
	<< m_ui->cppFileCheckBox->text() << ' '
	<< m_ui->fieldNamesGroupBox->windowTitle() << ' '
	<< m_ui->mutatorFirstCheckBox->text() << ' '
	<< m_ui->prefixLabel->text() << ' '
	<< m_ui->suffixLabel->text() << ' ';

    keywords.remove(QChar::fromLatin1('&'));

	return keywords;
}

SettingsPage::SettingsPage(QSharedPointer<Settings> &settings):
	m_settings(settings)
{
    setId(Constants::PAGE_ID);
    setDisplayName(QString::fromLatin1("Encapsulation"));
    setCategory(CppTools::Constants::CPP_SETTINGS_CATEGORY);
    setDisplayCategory(QCoreApplication::translate("CppTools", CppTools::Constants::CPP_SETTINGS_TR_CATEGORY));
    setCategoryIcon(QString::fromLatin1(CppTools::Constants::SETTINGS_CATEGORY_CPP_ICON));

}


QWidget *SettingsPage::createPage(QWidget *parent)
{
	m_widget = new SettingsWidget(parent);
	m_widget->setSettings(*m_settings);

	if(m_searchKeywords.isEmpty()) {
		m_searchKeywords = m_widget->searchKeywords();
	}

	return m_widget;
}

void SettingsPage::apply()
{
	if(m_widget) {
		const Settings newSettings = m_widget->settings();
		if(newSettings != *m_settings) {
			*m_settings = newSettings;
			m_settings->toSettings(Core::ICore::settings());
		}
	}
}

void SettingsPage::finish()
{
}

bool SettingsPage::matches(const QString &s) const
{
	return m_searchKeywords.contains(s, Qt::CaseInsensitive);
}

}
}
