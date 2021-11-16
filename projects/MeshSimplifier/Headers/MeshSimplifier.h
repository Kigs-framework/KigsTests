#pragma once

#include <DataDrivenBaseApplication.h>

class MeshSimplifier : public DataDrivenBaseApplication
{
public:
	DECLARE_CLASS_INFO(MeshSimplifier, DataDrivenBaseApplication, Core);
	DECLARE_CONSTRUCTOR(MeshSimplifier);

protected:
	void	ProtectedInit() override;
	void	ProtectedUpdate() override;
	void	ProtectedClose() override;

	
	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;
};
