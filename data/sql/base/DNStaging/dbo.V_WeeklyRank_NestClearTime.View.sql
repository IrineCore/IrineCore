USE [dnstaging]
GO
/****** Object:  View [dbo].[V_WeeklyRank_NestClearTime]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[V_WeeklyRank_NestClearTime]
AS
SELECT R.CharacterID1, DC1.CharacterName AS CharacterName1, R.CharacterLevel1, R.JobCode1, DJ1.JobName AS JobName1
	 , R.CharacterID2, DC2.CharacterName AS CharacterName2, R.CharacterLevel2, R.JobCode2, DJ2.JobName AS JobName2
	 , R.CharacterID3, DC3.CharacterName AS CharacterName3, R.CharacterLevel3, R.JobCode3, DJ3.JobName AS JobName3
	 , R.CharacterID4, DC4.CharacterName AS CharacterName4, R.CharacterLevel4, R.JobCode4, DJ4.JobName AS JobName4
	 , R.CharacterID5, DC5.CharacterName AS CharacterName5, R.CharacterLevel5, R.JobCode5, DJ5.JobName AS JobName5
	 , R.CharacterID6, DC6.CharacterName AS CharacterName6, R.CharacterLevel6, R.JobCode6, DJ6.JobName AS JobName6
	 , R.CharacterID7, DC7.CharacterName AS CharacterName7, R.CharacterLevel7, R.JobCode7, DJ7.JobName AS JobName7
	 , R.CharacterID8, DC8.CharacterName AS CharacterName8, R.CharacterLevel8, R.JobCode8, DJ8.JobName AS JobName8
	 , R.StageID, DS.StageName, R.ClearTime, R.TotalRanking
FROM dbo.WeeklyRank_NestClearTime R WITH (NOLOCK)
	LEFT OUTER JOIN dbo.DimCharacters DC1 WITH (NOLOCK) ON R.CharacterID1 = DC1.CharacterID
	LEFT OUTER JOIN dbo.DimCharacters DC2 WITH (NOLOCK) ON R.CharacterID2 = DC2.CharacterID
	LEFT OUTER JOIN dbo.DimCharacters DC3 WITH (NOLOCK) ON R.CharacterID3 = DC3.CharacterID
	LEFT OUTER JOIN dbo.DimCharacters DC4 WITH (NOLOCK) ON R.CharacterID4 = DC4.CharacterID
	LEFT OUTER JOIN dbo.DimCharacters DC5 WITH (NOLOCK) ON R.CharacterID5 = DC5.CharacterID
	LEFT OUTER JOIN dbo.DimCharacters DC6 WITH (NOLOCK) ON R.CharacterID6 = DC6.CharacterID
	LEFT OUTER JOIN dbo.DimCharacters DC7 WITH (NOLOCK) ON R.CharacterID7 = DC7.CharacterID
	LEFT OUTER JOIN dbo.DimCharacters DC8 WITH (NOLOCK) ON R.CharacterID8 = DC8.CharacterID
	LEFT OUTER JOIN dbo.DimJobs DJ1 WITH (NOLOCK) ON R.JobCode1 = DJ1.JobCode
	LEFT OUTER JOIN dbo.DimJobs DJ2 WITH (NOLOCK) ON R.JobCode2 = DJ2.JobCode
	LEFT OUTER JOIN dbo.DimJobs DJ3 WITH (NOLOCK) ON R.JobCode3 = DJ3.JobCode
	LEFT OUTER JOIN dbo.DimJobs DJ4 WITH (NOLOCK) ON R.JobCode4 = DJ4.JobCode
	LEFT OUTER JOIN dbo.DimJobs DJ5 WITH (NOLOCK) ON R.JobCode5 = DJ5.JobCode
	LEFT OUTER JOIN dbo.DimJobs DJ6 WITH (NOLOCK) ON R.JobCode6 = DJ6.JobCode
	LEFT OUTER JOIN dbo.DimJobs DJ7 WITH (NOLOCK) ON R.JobCode7 = DJ7.JobCode
	LEFT OUTER JOIN dbo.DimJobs DJ8 WITH (NOLOCK) ON R.JobCode8 = DJ8.JobCode
	INNER JOIN dbo.DimStages DS WITH (NOLOCK) ON R.StageID = DS.StageID
GO
