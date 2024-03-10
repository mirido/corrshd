#pragma once

class WithFinalInversion
{
	bool m_bDoFinalInversion;

public:
	WithFinalInversion();

	void setFinalInversionFlag(const bool bInvert);

	bool getFinalInversionFlag() const;

};
