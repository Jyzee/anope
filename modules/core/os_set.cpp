/* OperServ core functions
 *
 * (C) 2003-2011 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 */

/*************************************************************************/

#include "module.h"

class CommandOSSet : public Command
{
 private:
	void DoList(CommandSource &source)
	{
		Log(LOG_ADMIN, source.u, this);

		Anope::string index;

		index = readonly ? _("%s is enabled") : _("%s is disabled");
		source.Reply(index.c_str(), "READONLY");
		index = debug ? _("%s is enabled") : _("%s is disabled");
		source.Reply(index.c_str(), "DEBUG");
		index = noexpire ? _("%s is enabled") : _("%s is disabled");
		source.Reply(index.c_str(), "NOEXPIRE");

		return;
	}

	void DoSetReadOnly(CommandSource &source, const std::vector<Anope::string> &params)
	{
		User *u = source.u;
		const Anope::string &setting = params.size() > 1 ? params[1] : "";

		if (setting.empty())
		{
			this->OnSyntaxError(source, "READONLY");
			return;
		}

		if (setting.equals_ci("ON"))
		{
			readonly = true;
			Log(LOG_ADMIN, u, this) << "READONLY ON";
			source.Reply(_("Services are now in \002read-only\002 mode."));
		}
		else if (setting.equals_ci("OFF"))
		{
			readonly = false;
			Log(LOG_ADMIN, u, this) << "READONLY OFF";
			source.Reply(_("Services are now in \002read-write\002 mode."));
		}
		else
			source.Reply(_("Setting for READONLY must be \002on\002 or \002off\002."));

		return;
	}

	void DoSetSuperAdmin(CommandSource &source, const std::vector<Anope::string> &params)
	{
		User *u = source.u;
		const Anope::string &setting = params.size() > 1 ? params[1] : "";

		if (setting.empty())
		{
			this->OnSyntaxError(source, "SUPERADMIN");
			return;
		}

		/**
		 * Allow the user to turn super admin on/off
		 *
		 * Rob
		 **/
		if (!Config->SuperAdmin)
			source.Reply(_("SuperAdmin setting not enabled in services.conf"));
		else if (setting.equals_ci("ON"))
		{
			u->isSuperAdmin = 1;
			source.Reply(_("You are now a SuperAdmin"));
			Log(LOG_ADMIN, u, this) << "SUPERADMIN ON";
			ircdproto->SendGlobops(source.owner, _("%s is now a Super-Admin"), u->nick.c_str());
		}
		else if (setting.equals_ci("OFF"))
		{
			u->isSuperAdmin = 0;
			source.Reply(_("You are no longer a SuperAdmin"));
			Log(LOG_ADMIN, u, this) << "SUPERADMIN OFF";
			ircdproto->SendGlobops(source.owner, _("%s is no longer a Super-Admin"), u->nick.c_str());
		}
		else
			source.Reply(_("Setting for SuperAdmin must be \002on\002 or \002off\002 (must be enabled in services.conf)"));

		return;
	}

	void DoSetDebug(CommandSource &source, const std::vector<Anope::string> &params)
	{
		User *u = source.u;
		const Anope::string &setting = params.size() > 1 ? params[1] : "";

		if (setting.empty())
		{
			this->OnSyntaxError(source, "DEBUG");
			return;
		}

		if (setting.equals_ci("ON"))
		{
			debug = 1;
			Log(LOG_ADMIN, u, this) << "DEBUG ON";
			source.Reply(_("Services are now in debug mode."));
		}
		else if (setting.equals_ci("OFF") || (setting[0] == '0' && setting.is_number_only() && !convertTo<int>(setting)))
		{
			Log(LOG_ADMIN, u, this) << "DEBUG OFF";
			debug = 0;
			source.Reply(_("Services are now in non-debug mode."));
		}
		else
		{
			try
			{
				debug = convertTo<int>(setting);
				Log(LOG_ADMIN, u, this) << "DEBUG " << debug;
				source.Reply(_("Services are now in debug mode (level %d)."), debug);
				return;
			}
			catch (const ConvertException &) { }
			
			source.Reply(_("Setting for DEBUG must be \002ON\002, \002OFF\002, or a positive number."));
		}

		return;
	}

	void DoSetNoExpire(CommandSource &source, const std::vector<Anope::string> &params)
	{
		User *u = source.u;
		const Anope::string &setting = params.size() > 1 ? params[1] : "";

		if (setting.empty())
		{
			this->OnSyntaxError(source, "NOEXPIRE");
			return;
		}

		if (setting.equals_ci("ON"))
		{
			noexpire = true;
			Log(LOG_ADMIN, u, this) << "NOEXPIRE ON";
			source.Reply(_("Services are now in \002no expire\002 mode."));
		}
		else if (setting.equals_ci("OFF"))
		{
			noexpire = false;
			Log(LOG_ADMIN, u, this) << "NOEXPIRE OFF";
			source.Reply(_("Services are now in \002expire\002 mode."));
		}
		else
			source.Reply(_("Setting for NOEXPIRE must be \002on\002 or \002off\002."));

		return;
	}
 public:
	CommandOSSet(Module *creator) : Command(creator, "operserv/set", 1, 2)
	{
		this->SetDesc(_("Set various global Services options"));
		this->SetSyntax(_("\037option\037 \037setting\037"));
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		const Anope::string &option = params[0];

		if (option.equals_ci("LIST"))
			return this->DoList(source);
		else if (option.equals_ci("READONLY"))
			return this->DoSetReadOnly(source, params);
		else if (option.equals_ci("SUPERADMIN"))
			return this->DoSetSuperAdmin(source, params);
		else if (option.equals_ci("DEBUG"))
			return this->DoSetDebug(source, params);
		else if (option.equals_ci("NOEXPIRE"))
			return this->DoSetNoExpire(source, params);
		else
			source.Reply(_("Unknown option \002%s\002."), option.c_str());

		return;
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{
		if (subcommand.empty())
		{
			this->SendSyntax(source);
			source.Reply(" ");
			source.Reply(_("Sets various global Services options.  Option names\n"
					"currently defined are:\n"
					"    READONLY   Set read-only or read-write mode\n"
					"    DEBUG      Activate or deactivate debug mode\n"
					"    NOEXPIRE   Activate or deactivate no expire mode\n"
					"    SUPERADMIN Activate or deactivate super-admin mode\n"
					"    LIST       List the options"));
		}
		else if (subcommand.equals_ci("LIST"))
			source.Reply(_("Syntax: \002LIST\n"
					"Display the various %s settings"), Config->OperServ.c_str());
		else if (subcommand.equals_ci("READONLY"))
			source.Reply(_("Syntax: \002READONLY {ON | OFF}\002\n"
					" \n"
					"Sets read-only mode on or off.  In read-only mode, normal\n"
					"users will not be allowed to modify any Services data,\n"
					"including channel and nickname access lists, etc.  IRCops\n"
					"with sufficient Services privileges will be able to modify\n"
					"Services' AKILL list and drop or forbid nicknames and\n"
					"channels, but any such changes will not be saved unless\n"
					"read-only mode is deactivated before Services is terminated\n"
					"or restarted.\n"
					" \n"
					"This option is equivalent to the command-line option\n"
					"\002-readonly\002."));
		else if (subcommand.equals_ci("NOEXPIRE"))
			source.Reply(_("Syntax: \002NOEXPIRE {ON | OFF}\002\n"
					"Sets no expire mode on or off. In no expire mode, nicks,\n"
					"channels, akills and exceptions won't expire until the\n"
					"option is unset.\n"
					"This option is equivalent to the command-line option\n"
					"\002-noexpire\002."));
		else if (subcommand.equals_ci("SUPERADMIN"))
			source.Reply(_("Syntax: \002SUPERADMIN {ON | OFF}\002\n"
					"Setting this will grant you extra privileges such as the\n"
					"ability to be \"founder\" on all channel's etc...\n"
					"This option is \002not\002 persistent, and should only be used when\n"
					"needed, and set back to OFF when no longer needed."));
		else
			return false;
		return true;
	}
};

class OSSet : public Module
{
	CommandOSSet commandosset;

 public:
	OSSet(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, CORE),
		commandosset(this)
	{
		this->SetAuthor("Anope");

		ModuleManager::RegisterService(&commandosset);
	}
};

MODULE_INIT(OSSet)
