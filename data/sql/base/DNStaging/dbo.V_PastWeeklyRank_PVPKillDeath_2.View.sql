USE [dnstaging]
GO
/****** Object:  View [dbo].[V_PastWeeklyRank_PVPKillDeath_2]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[V_PastWeeklyRank_PVPKillDeath_2]
AS
SELECT R.BasisDate, R.CharacterID, DC.CharacterName, R.WorldID, DW.WorldName, R.CharacterLevel, R.JobCode, DJ.JobName, R.PVPLevel, R.KillCount, R.DeathCount, R.vsWarriorKOWin, R.vsWarriorKOLose, R.vsArcherKOWin, R.vsArcherKOLose, R.vsSorceressKOWin, R.vsSorceressKOLose, R.vsClericKOWin, R.vsClericKOLose, R.TotalKillRank, R.WorldKillRank, R.WorldCharacterClassKillRank, R.CharacterClassKillRank, R.TotalDeathRank, R.WorldDeathRank, R.WorldCharacterClassDeathRank, R.CharacterClassDeathRank, R.vsAcademicKOWin, R.vsAcademicKOLose
FROM dbo.PastWeeklyRank_PVPKillDeath R WITH (NOLOCK)
	INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON R.CharacterID = DC.CharacterID
	INNER JOIN dbo.DimJobs DJ WITH (NOLOCK) ON R.JobCode = DJ.JobCode
	INNER JOIN dbo.DimWorlds DW WITH (NOLOCK) ON R.WorldID = DW.WorldID
GO
