#include "stdafx.h"
#include "WithFinalInversion.h"

WithFinalInversion::WithFinalInversion()
	: m_bDoFinalInversion(true)
{
	/*pass*/
}

void WithFinalInversion::doFinalInversion(const bool bDo)
{
	m_bDoFinalInversion = bDo;
}
