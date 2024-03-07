#include "framework.h"

std::string ExtractFileName(const std::string& fullPath)
{
	// 경로에서 역슬래시 또는 슬래시를 찾아서 경로 끝을 찾습니다.
	size_t lastPathSeparator = fullPath.find_last_of("\\/");

	// 경로 끝 이후의 문자열이 파일명입니다.
	std::string fileName = fullPath.substr(lastPathSeparator + 1);

	// 파일명에서 마지막 점을 찾아서 확장자 끝을 찾습니다.
	size_t lastDot = fileName.find_last_of('.');

	// 마지막 점 이후의 문자열이 확장자입니다.
	std::string fileNameWithoutExtension = fileName.substr(0, lastDot);

	return fileNameWithoutExtension;
}