#ifndef ENCAPSULATIONSETTINGSPAGE_H
#define ENCAPSULATIONSETTINGSPAGE_H

#include <coreplugin/dialogs/ioptionspage.h>

#include <QtCore/QPointer>
#include <QtGui/QWidget>

class QSettings;

namespace Encapsulation
{
	namespace Internal
	{
		namespace Ui
		{
			class SettingsWidget;
		}

		struct Settings
		{
			QString fieldPrefix;
			QString fieldSuffix;

			bool addGetString;
			bool useCamelCase;
			bool mutatorFirst;
			bool cppFile;

			void toSettings(QSettings *) const;
			void fromSettings(QSettings *);

			bool equals(const Settings &rhs) const;
			bool operator==(const Settings &s) const { return equals(s); }
			bool operator!=(const Settings &s) const { return !equals(s); }
		};

		class SettingsWidget: public QWidget
		{
			Q_OBJECT

			public:
				explicit SettingsWidget(QWidget *parent = 0);

				Settings settings() const;
				void setSettings(const Settings &s);

				QString searchKeywords() const;

			private:
				Ui::SettingsWidget *m_ui;
		};

		class SettingsPage: public Core::IOptionsPage
		{
			public:
				SettingsPage(QSharedPointer<Settings> &settings);

				virtual QString id() const;
				virtual QString displayName() const;
				virtual QString category() const;
				virtual QString displayCategory() const;
				virtual QIcon categoryIcon() const;

				virtual QWidget *createPage(QWidget *parent);
				virtual void apply();
				virtual void finish();
				virtual bool matches(const QString &) const;

			private:
				const QSharedPointer<Settings> m_settings;
				QPointer<SettingsWidget> m_widget;
				QString m_searchKeywords;
		};
	}
}

#endif // ENCAPSULATIONSETTINGSPAGE_H
