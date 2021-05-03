#pragma once

#include <DataDrivenBaseApplication.h>

class testPixelSprites : public DataDrivenBaseApplication
{
public:
	DECLARE_CLASS_INFO(testPixelSprites, DataDrivenBaseApplication, Core);
	DECLARE_CONSTRUCTOR(testPixelSprites);

protected:
	void	ProtectedInit() override;
	void	ProtectedUpdate() override;
	void	ProtectedClose() override;

	
	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;
};
