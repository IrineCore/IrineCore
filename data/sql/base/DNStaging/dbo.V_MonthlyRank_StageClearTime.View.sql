USE [dnstaging]
GO
/****** Object:  View [dbo].[V_MonthlyRank_StageClearTime]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[V_MonthlyRank_StageClearTime]
AS
SELECT R.CharacterID, DC.CharacterName, R.WorldID, DW.WorldName, R.StageID, DS.StageName, R.CharacterLevel, R.JobCode, DJ.JobName, R.ClearTime, R.ClearDate, R.TotalRank, R.WorldRank, R.WorldCharacterClassRank, R.CharacterClassRank
FROM dbo.MonthlyRank_StageClearTime R WITH (NOLOCK)
	INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON R.CharacterID = DC.CharacterID
	INNER JOIN dbo.DimJobs DJ WITH (NOLOCK) ON R.JobCode = DJ.JobCode
	INNER JOIN dbo.DimWorlds DW WITH (NOLOCK) ON R.WorldID = DW.WorldID
	INNER JOIN dbo.DimStages DS WITH (NOLOCK) ON R.StageID = DS.StageID
GO
