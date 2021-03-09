#pragma once

#include <DataDrivenBaseApplication.h>

class testXLSX : public DataDrivenBaseApplication
{
public:
	DECLARE_CLASS_INFO(testXLSX, DataDrivenBaseApplication, Core);
	DECLARE_CONSTRUCTOR(testXLSX);

protected:
	void	ProtectedInit() override;
	void	ProtectedUpdate() override;
	void	ProtectedClose() override;

	
	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;
};
