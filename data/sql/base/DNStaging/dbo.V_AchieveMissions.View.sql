USE [dnstaging]
GO
/****** Object:  View [dbo].[V_AchieveMissions]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[V_AchieveMissions]
AS
SELECT DA.AccountID, DA.AccountName, DC.WorldID, DC.CharacterID, DC.CharacterName, DM.MissionID, DM.MissionName, DM.MissionScore
FROM dbo.SS_AchieveMissions SS WITH (NOLOCK)
	INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON SS.CharacterID = DC.CharacterID
	INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
	INNER JOIN dbo.DimMissions DM WITH (NOLOCK) ON SS.MissionID = DM.MissionID
WHERE DC.DeleteFlag = CAST(0 AS bit)
GO
