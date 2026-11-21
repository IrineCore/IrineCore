USE [dnstaging]
GO
/****** Object:  View [dbo].[V_Rank_MissionScore]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[V_Rank_MissionScore]
AS
SELECT R.CharacterID, DC.CharacterName, R.WorldID, DW.WorldName, R.CharacterLevel, R.JobCode, DJ.JobName, R.MissionScore, R.TotalRank, R.WorldRank, R.WorldCharacterClassRank, R.CharacterClassRank
FROM dbo.Rank_MissionScore R WITH (NOLOCK)
	INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON R.CharacterID = DC.CharacterID
	INNER JOIN dbo.DimJobs DJ WITH (NOLOCK) ON R.JobCode = DJ.JobCode
	INNER JOIN dbo.DimWorlds DW WITH (NOLOCK) ON R.WorldID = DW.WorldID
GO
