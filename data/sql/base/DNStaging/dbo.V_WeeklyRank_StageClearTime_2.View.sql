USE [dnstaging]
GO
/****** Object:  View [dbo].[V_WeeklyRank_StageClearTime_2]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[V_WeeklyRank_StageClearTime_2]
AS
SELECT R.CharacterID, DC.CharacterName, R.WorldID, DW.WorldName, R.StageID, DS.StageName, R.CharacterLevel, R.JobCode, DJ.JobName, R.ClearTime, R.ClearDate, R.TotalRank, R.WorldRank, R.WorldCharacterClassRank, R.CharacterClassRank, DJ2.JobCode AS CharacterClassCode, DJ2.JobName AS CharacterClassName
FROM dbo.WeeklyRank_StageClearTime R WITH (NOLOCK)
	INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON R.CharacterID = DC.CharacterID
	INNER JOIN dbo.DimJobs DJ WITH (NOLOCK) ON R.JobCode = DJ.JobCode
	INNER JOIN dbo.DimWorlds DW WITH (NOLOCK) ON R.WorldID = DW.WorldID
	INNER JOIN dbo.DimStages DS WITH (NOLOCK) ON R.StageID = DS.StageID
	INNER JOIN dbo.DimJobs DJ2 WITH (NOLOCK) ON DC.CharacterClassCode = DJ2.JobCode
GO
