// Made with XBLAH's Modding tool.
// Download it at https://github.com/jean-knapp/windows-source-modding-tool/releases

"gamemenu"
{
	"0"
	{
		"label"	"Resume game"
		"command"	"ResumeGame"
		"ingameorder"	"0"
		"onlyingame"	"1"
	}
	"1"
	{
		"label"	"New game"
		"command"	"OpenNewGameDialog"
		"ingameorder"	"1"
		"nomulti"	"1"
	}
	"2"
	{
		"label"	"Load game"
		"command"	"OpenLoadGameDialog"
		"ingameorder"	"2"
		"nomulti"	"1"
	}
	"3"
	{
		"label"	"Save game"
		"command"	"OpenSaveGameDialog"
		"ingameorder"	"3"
		"onlyingame"	"1"
		"nomulti"	"1"
	}
	"4"
	{
		"label"	"Options"
		"command"	"OpenOptionsDialog"
		"ingameorder"	"4"
	}
	"5"
	{
		"label"	"Quit"
		"command"	"Quit"
		"ingameorder"	"5"
	}
}
