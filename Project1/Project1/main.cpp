#include "CSid.h"
//#include "audio/arkanoid.h"
#include "audio/commando.h"

int main(int argc, char* argv[])
{
	CSid CSid;

	char filePath[] = "test.sid";

	CSid.Init();
	CSid.LoadFile(filePath);

	//CSid.InserFile(music_Commando_sid, music_Commando_sid_len);

	CSid.Play();

	printf("Press Enter to abort playback\n");
	getchar();

	CSid.Stop();

	return 0;
}
