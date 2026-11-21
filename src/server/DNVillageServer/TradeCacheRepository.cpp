
#include "stdafx.h"
#include "TradeCacheRepository.h"

using namespace TradeSystem;

CMarketPriceCache::CMarketPriceCache()
{
	m_tLastResetTime.Reset();
	ClearCache();
}

bool CMarketPriceCache::CheckResetCache ()
{
	CTimeSet tCurrentTime;

	if (m_tLastResetTime.GetDay () != tCurrentTime.GetDay())
	{
		m_tLastResetTime = tCurrentTime;
		return true;
	}

	return false;
}

bool CMarketPriceCache::GetTradePrice (int nItemID, BYTE cLevel, BYTE cOption, TMarketPrice* pPrices)
{
	// Check if it is reset time.
	if (CheckResetCache ())
	{
		ClearCache();		// Delete cash at 12:00 sharp
		return false;
	}
	
	ScopeLock<CSyncLock> Lock( m_Sync );

	MTradePriceItor itor = m_mTradePrice.find (nItemID);
	if (itor != m_mTradePrice.end())
	{	
		std::vector<TMarketPriceCacheData>& VecCacheData = (*itor).second;
		std::vector<TMarketPriceCacheData>::const_iterator cache_itor = VecCacheData.begin();
		for (; cache_itor != VecCacheData.end(); cache_itor++)
		{
			if ((*cache_itor).cLevel == cLevel && (*cache_itor).cOption == cOption)
			{
				memcpy (pPrices, (*cache_itor).ItemPrices, sizeof((*cache_itor).ItemPrices));
				return true;
			}
		}
	}

	return false;
}

void CMarketPriceCache::SetTradePrice (int nItemID, BYTE cLevel, BYTE cOption, TMarketPrice* pPrices)
{
	ScopeLock<CSyncLock> Lock( m_Sync );

	TMarketPriceCacheData Data;
	Data.cLevel = cLevel;
	Data.cOption = cOption;
	memcpy (Data.ItemPrices, pPrices, sizeof(Data.ItemPrices));

	MTradePriceItor itor = m_mTradePrice.find (nItemID);
	if (itor != m_mTradePrice.end())
	{
		std::vector<TMarketPriceCacheData>& VecCacheData = (*itor).second;
		std::vector<TMarketPriceCacheData>::const_iterator cache_itor = VecCacheData.begin();

		bool bIsAdd = true;
		for (; cache_itor != VecCacheData.end(); cache_itor++)
		{
			if ((*cache_itor).cLevel == cLevel && (*cache_itor).cOption == cOption)
				bIsAdd = false;
		}

		if (bIsAdd)
			VecCacheData.push_back (Data);
	}
	else
	{
		std::vector<TMarketPriceCacheData> vList;
		vList.push_back(Data);

		m_mTradePrice.insert (make_pair(nItemID, vList));
	}
}

void CMarketPriceCache::ClearCache()
{
	ScopeLock<CSyncLock> Lock( m_Sync );

	for( MTradePriceItor itor = m_mTradePrice.begin() ; itor!=m_mTradePrice.end() ; itor++)
		(*itor).second.clear();

	m_mTradePrice.clear();
}
