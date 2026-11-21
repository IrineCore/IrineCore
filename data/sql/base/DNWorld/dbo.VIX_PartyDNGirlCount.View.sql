USE [dnworld]
GO
/****** Object:  View [dbo].[VIX_PartyDNGirlCount]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[VIX_PartyDNGirlCount]
WITH SCHEMABINDING AS
SELECT PartyID, COUNT_BIG(*) AS PartyDNGirlCount
FROM dbo.PartyJoins PJ WITH(NOLOCK) 
	LEFT OUTER JOIN dbo.Characters CH WITH(NOLOCK) ON PJ.CharacterID = CH.CharacterID
WHERE CH.AccountLevelCode = 120
GROUP BY PartyID;
GO
