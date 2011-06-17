/* NickServ core functions
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
#include "nickserv.h"

static bool SendPassMail(User *u, NickAlias *na, const Anope::string &pass);

class CommandNSSendPass : public Command
{
 public:
	CommandNSSendPass() : Command("SENDPASS", 1, 1)
	{
		this->SetFlag(CFLAG_ALLOW_UNREGISTERED);
		this->SetDesc(_("Forgot your password? Try this"));
	}

	CommandReturn Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		User *u = source.u;
		const Anope::string &nick = params[0];
		NickAlias *na;

		if (Config->RestrictMail && (!u->Account() || !u->HasCommand("nickserv/sendpass")))
			source.Reply(_(ACCESS_DENIED));
		else if (!(na = findnick(nick)))
			source.Reply(_(NICK_X_NOT_REGISTERED), nick.c_str());
		else
		{
			Anope::string tmp_pass;
			if (enc_decrypt(na->nc->pass, tmp_pass) == 1)
			{
				if (SendPassMail(u, na, tmp_pass))
				{
					Log(Config->RestrictMail ? LOG_ADMIN : LOG_COMMAND, u, this) << "for " << na->nick;
					source.Reply(_("Password of \002%s\002 has been sent."), nick.c_str());
				}
			}
			else
				source.Reply(_("SENDPASS command unavailable because encryption is in use."));
		}

		return MOD_CONT;
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{
		source.Reply(_("Syntax: \002SENDPASS \037nickname\037\002\n"
				" \n"
				"Send the password of the given nickname to the e-mail address\n"
				"set in the nickname record. This command is really useful\n"
				"to deal with lost passwords.\n"
				" \n"
				"May be limited to \002IRC operators\002 on certain networks."));
		return true;
	}

	void OnSyntaxError(CommandSource &source, const Anope::string &subcommand)
	{
		SyntaxError(source, "SENDPASS", _("SENDPASS \037nickname\037"));
	}
};

class NSSendPass : public Module
{
	CommandNSSendPass commandnssendpass;

 public:
	NSSendPass(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, CORE)
	{
		this->SetAuthor("Anope");

		if (!nickserv)
			throw ModuleException("NickServ is not loaded!");

		if (!Config->UseMail)
			throw ModuleException("Not using mail, whut.");

		Anope::string tmp_pass = "plain:tmp";
		if (enc_decrypt(tmp_pass, tmp_pass) == -1)
			throw ModuleException("Incompatible with the encryption module being used");

		this->AddCommand(nickserv->Bot(), &commandnssendpass);
	}
};

static bool SendPassMail(User *u, NickAlias *na, const Anope::string &pass)
{
	char subject[BUFSIZE], message[BUFSIZE];

	snprintf(subject, sizeof(subject), translate(na->nc, _("Nickname password (%s)")), na->nick.c_str());
	snprintf(message, sizeof(message), translate(na->nc, _(
	"Hi,\n"
	" \n"
	"You have requested to receive the password of nickname %s by e-mail.\n"
	"The password is %s. For security purposes, you should change it as soon as you receive this mail.\n"
	" \n"
	"If you don't know why this mail was sent to you, please ignore it silently.\n"
	" \n"
	"%s administrators.")), na->nick.c_str(), pass.c_str(), Config->NetworkName.c_str());

	return Mail(u, na->nc, nickserv->Bot(), subject, message);
}

MODULE_INIT(NSSendPass)
