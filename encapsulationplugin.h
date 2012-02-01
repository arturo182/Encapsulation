#ifndef ENCAPSULATION_H
#define ENCAPSULATION_H

#include "encapsulation_global.h"

#include <extensionsystem/iplugin.h>

namespace Encapsulation
{
	namespace Internal
	{
		struct Settings;

		class Plugin : public ExtensionSystem::IPlugin
		{
				Q_OBJECT

			public:
				Plugin();
				~Plugin();

				bool initialize(const QStringList &arguments, QString *errorString);
				void extensionsInitialized();
				ShutdownFlag aboutToShutdown();

			private slots:
				void triggerAction();

			private:
				QSharedPointer<Settings> m_settings;
		};

	} // namespace Internal
} // namespace Encapsulation

#endif // ENCAPSULATION_H

