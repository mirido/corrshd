#include "stdafx.h"
#include "WithFinalInversion.h"

WithFinalInversion::WithFinalInversion()
	: m_bDoFinalInversion(true)
{
	/*pass*/
}

void WithFinalInversion::setFinalInversionFlag(const bool bInvert)
{
	m_bDoFinalInversion = bInvert;
}

bool WithFinalInversion::getFinalInversionFlag() const
{
	return m_bDoFinalInversion;
}
