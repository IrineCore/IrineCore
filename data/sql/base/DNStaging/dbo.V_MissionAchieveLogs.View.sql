USE [dnstaging]
GO
/****** Object:  View [dbo].[V_MissionAchieveLogs]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE VIEW [dbo].[V_MissionAchieveLogs]
	AS
	SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20130913 UNION ALL
SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20130914 UNION ALL
SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20130915 UNION ALL
SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20130916 UNION ALL
SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20130917 UNION ALL
SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20130918 UNION ALL
SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20130919 UNION ALL
SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20130920 UNION ALL
SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20130921 UNION ALL
SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20131003 UNION ALL
SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20131004 UNION ALL
SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20131005 UNION ALL
SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20131006 UNION ALL
SELECT * FROM DNStaging.dbo.MissionAchieveLogs_20131007
GO
