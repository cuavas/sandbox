#include <CoreFoundation/CoreFoundation.h>

#include <cstdio>
#include <cstring>
#include <iostream>


int main(int argc, char *argv[])
{
	char const fname[] = "test.plist";
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(
			nullptr,
			(UInt8 const *)fname,
			strlen(fname),
			false);
	if (!url)
	{
		std::cerr << "Error creating URL." << std::endl;
		return 1;
	}

	CFNumberRef sizeprop = nullptr;
	if (!CFURLCopyResourcePropertyForKey(url, kCFURLFileSizeKey, &sizeprop, nullptr))
	{
		CFRelease(url);
		std::cerr << "Error getting file size." << std::endl;
		return 1;
	}

	CFIndex size;
	if (!CFNumberGetValue(sizeprop, kCFNumberCFIndexType, &size))
	{
		CFRelease(url);
		CFRelease(sizeprop);
		std::cerr << "Error getting file size." << std::endl;
		return 1;
	}
	CFRelease(sizeprop);
	sizeprop = nullptr;

	CFReadStreamRef stream = CFReadStreamCreateWithFile(nullptr, url);
	CFRelease(url);
	url = nullptr;
	if (!stream)
	{
		std::cerr << "Error creating stream." << std::endl;
		return 1;
	}
	if (!CFReadStreamOpen(stream))
	{
		CFRelease(stream);
		std::cerr << "Error opening stream." << std::endl;
		return 1;
	}

	CFMutableDataRef data = CFDataCreateMutable(nullptr, size);
	if (!data)
	{
		CFRelease(stream);
		std::cerr << "Error allocating data." << std::endl;
		return 1;
	}
	CFDataSetLength(data, size);
	UInt8 *const bytes = CFDataGetMutableBytePtr(data);

	for (CFIndex read = 0; size > read; )
	{
		CFIndex const chunk = CFReadStreamRead(stream, bytes + read, size - read);
		if (0 > chunk)
		{
			CFReadStreamClose(stream);
			CFRelease(stream);
			CFRelease(data);
			std::cerr << "Error reading file." << std::endl;
			return 1;
		}
		else if (0 == chunk)
		{
			CFReadStreamClose(stream);
			CFRelease(stream);
			CFRelease(data);
			std::cerr << "Unexpected EOF." << std::endl;
			return 1;
		}
		read += chunk;
	}
	CFReadStreamClose(stream);
	CFRelease(stream);
	stream = nullptr;

	std::cout << "Read " << size << " bytes." << std::endl;

	CFErrorRef msg = nullptr;
	CFPropertyListRef const result = CFPropertyListCreateWithData(
			nullptr,
			data,
			kCFPropertyListImmutable,
			nullptr,
			&msg);
	if (!result || msg)
	{
		std::cerr << "Error creating property list." << std::endl;
		if (msg)
			CFRelease(msg);
		if (result)
			CFRelease(result);
		return 1;
	}

	CFRelease(result);

	return 0;
}
