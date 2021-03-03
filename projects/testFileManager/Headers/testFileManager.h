#pragma once

#include <DataDrivenBaseApplication.h>

class testFileManager : public DataDrivenBaseApplication
{
public:
	DECLARE_CLASS_INFO(testFileManager, DataDrivenBaseApplication, Core);
	DECLARE_CONSTRUCTOR(testFileManager);

protected:
	void	ProtectedInit() override;
	void	ProtectedUpdate() override;
	void	ProtectedClose() override;

	
	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;
};
