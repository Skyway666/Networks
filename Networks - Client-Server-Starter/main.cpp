#include "Application.h"


// NOTE(jesus):
// The following line avoids the black console from appearing.
// It can also be configured in the project linker settings.
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

Application * App = nullptr;

enum class MainState
{
	Create,
	Init,
	Loop,
	CleanUp,
	Fail,
	Exit
};

#define PAUSE_AND_EXIT() system("pause"); exit(-1)

void printWSErrorAndExit(const char *msg)
{
	wchar_t *s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);
	PAUSE_AND_EXIT();
}

int main(int argc, char **argv)
{
	int result = EXIT_FAILURE;

	MainState state = MainState::Create;

	while (state != MainState::Exit)
	{
		switch (state)
		{
		case MainState::Create:
			App = new Application();
			if (App != nullptr) {
				state = MainState::Init;
			} else {
				LOG("Create failed");
				state = MainState::Fail;
			}
			break;

		case MainState::Init:
			if (App->init()) {
				state = MainState::Loop;
			} else {
				LOG("Start failed");
				state = MainState::Fail;
			}
			break;

		case MainState::Loop:
			if (App->update()) {
				// Update ok, continue here...
			} else {
				state = MainState::CleanUp;
			}
			break;

		case MainState::CleanUp:
			if (App->cleanUp()) {
				delete App;
				App = nullptr;
				state = MainState::Exit;
				result = EXIT_SUCCESS;
			} else {
				LOG("CleanUp failed");
				state = MainState::Fail;
			}
			break;

		case MainState::Fail:
			LOG("Application failed :-(");
			state = MainState::Exit;
			break;

		case MainState::Exit:
			break;
		default:

			break;
		}
	}

	return result;
}
