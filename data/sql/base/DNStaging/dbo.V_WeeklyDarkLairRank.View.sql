USE [dnstaging]
GO
/****** Object:  View [dbo].[V_WeeklyDarkLairRank]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[V_WeeklyDarkLairRank]
AS
SELECT DLR.WorldID, DLR.MapID, DLR.PartySize, DLR.PartyName, DLR.PlayRound, DLR.PlayTime, DLR.LogDate, DLR.WorldRank, DLR.TotalRank, DC1.CharacterName AS CharacterName1, DJ1.JobName AS JobName1, DC2.CharacterName AS CharacterName2, DJ2.JobName AS JobName2, DC3.CharacterName AS CharacterName3, DJ3.JobName AS JobName3 , DC4.CharacterName AS CharacterName4, DJ4.JobName AS JobName4
FROM dbo.WeeklyDarkLairRank DLR WITH(NOLOCK)
	LEFT OUTER JOIN dbo.DimCharacters DC1 WITH(NOLOCK) ON DLR.CharacterID1 = DC1.CharacterID
	LEFT OUTER JOIN dbo.DimCharacters DC2 WITH(NOLOCK) ON DLR.CharacterID2 = DC2.CharacterID
	LEFT OUTER JOIN dbo.DimCharacters DC3 WITH(NOLOCK) ON DLR.CharacterID3 = DC3.CharacterID
	LEFT OUTER JOIN dbo.DimCharacters DC4 WITH(NOLOCK) ON DLR.CharacterID4 = DC4.CharacterID
	LEFT OUTER JOIN dbo.DimJobs DJ1 WITH(NOLOCK) ON DLR.JobCode1 = DJ1.JobCode
	LEFT OUTER JOIN dbo.DimJobs DJ2 WITH(NOLOCK) ON DLR.JobCode2 = DJ2.JobCode
	LEFT OUTER JOIN dbo.DimJobs DJ3 WITH(NOLOCK) ON DLR.JobCode3 = DJ3.JobCode
	LEFT OUTER JOIN dbo.DimJobs DJ4 WITH(NOLOCK) ON DLR.JobCode4 = DJ4.JobCode
GO
