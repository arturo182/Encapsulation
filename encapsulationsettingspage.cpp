#include "encapsulationsettingspage.h"
#include "ui_encapsulationsettingswidget.h"
#include "encapsulationconstants.h"

#include <cpptools/cpptoolsconstants.h>
#include <coreplugin/icore.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QSettings>

namespace Encapsulation
{
namespace Internal
{

void Settings::toSettings(QSettings *s) const
{
	s->beginGroup(Constants::SETTINGS_GROUP);
	s->setValue(Constants::SETTING_PREFIX, fieldPrefix);
	s->setValue(Constants::SETTING_SUFFIX, fieldSuffix);
	s->setValue(Constants::SETTING_GETSTRING, addGetString);
	s->setValue(Constants::SETTING_CCASE, useCamelCase);
	s->setValue(Constants::SETTING_MUTFIRST, mutatorFirst);
	s->setValue(Constants::SETTING_CPPFILE, cppFile);
	s->endGroup();
}

void Settings::fromSettings(QSettings *s)
{
	s->beginGroup(Constants::SETTINGS_GROUP);
	fieldPrefix = s->value(Constants::SETTING_PREFIX, "m_").toString();
	fieldSuffix = s->value(Constants::SETTING_SUFFIX).toString();
	addGetString = s->value(Constants::SETTING_GETSTRING, false).toBool();
	useCamelCase = s->value(Constants::SETTING_CCASE, true).toBool();
	mutatorFirst = s->value(Constants::SETTING_MUTFIRST, true).toBool();
	cppFile = s->value(Constants::SETTING_CPPFILE, false).toBool();
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
	m_ui->cppFileCheckBox->hide();
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

	keywords.remove('&');

	return keywords;
}

SettingsPage::SettingsPage(QSharedPointer<Settings> &settings):
	m_settings(settings)
{
}

QString SettingsPage::id() const
{
	return Constants::PAGE_ID;
}

QString SettingsPage::displayName() const
{
	return "Encapsulation";
}

QString SettingsPage::category() const
{
	return CppTools::Constants::CPP_SETTINGS_CATEGORY;
}

QString SettingsPage::displayCategory() const
{
	return QCoreApplication::translate("CppTools", CppTools::Constants::CPP_SETTINGS_TR_CATEGORY);
}

QIcon SettingsPage::categoryIcon() const
{
	return QIcon(CppTools::Constants::SETTINGS_CATEGORY_CPP_ICON);
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
