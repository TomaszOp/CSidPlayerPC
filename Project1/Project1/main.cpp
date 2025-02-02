#include "CSid.h"
//#include "audio/arkanoid.h"
#include "audio/test_sid.h"

int main(int argc, char* argv[])
{
	CSid CSid;

	char filePath[] = "test.sid";

	CSid.Init();
	//CSid.LoadFile(filePath);

	CSid.InserFile(test_sid, test_sid_len);

	CSid.Play();

	printf("Press Enter to abort playback\n");
	getchar();

	CSid.Stop();

	return 0;
}
