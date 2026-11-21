USE [dnstaging]
GO
/****** Object:  View [dbo].[V_CharacterActionLogs]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE VIEW [dbo].[V_CharacterActionLogs]
	AS
	SELECT * FROM DNStaging.dbo.CharacterActionLogs_20130913 UNION ALL
SELECT * FROM DNStaging.dbo.CharacterActionLogs_20130914 UNION ALL
SELECT * FROM DNStaging.dbo.CharacterActionLogs_20130915 UNION ALL
SELECT * FROM DNStaging.dbo.CharacterActionLogs_20130916 UNION ALL
SELECT * FROM DNStaging.dbo.CharacterActionLogs_20130917 UNION ALL
SELECT * FROM DNStaging.dbo.CharacterActionLogs_20130918 UNION ALL
SELECT * FROM DNStaging.dbo.CharacterActionLogs_20130919 UNION ALL
SELECT * FROM DNStaging.dbo.CharacterActionLogs_20130920 UNION ALL
SELECT * FROM DNStaging.dbo.CharacterActionLogs_20130921 UNION ALL
SELECT * FROM DNStaging.dbo.CharacterActionLogs_20131003 UNION ALL
SELECT * FROM DNStaging.dbo.CharacterActionLogs_20131004 UNION ALL
SELECT * FROM DNStaging.dbo.CharacterActionLogs_20131005 UNION ALL
SELECT * FROM DNStaging.dbo.CharacterActionLogs_20131006 UNION ALL
SELECT * FROM DNStaging.dbo.CharacterActionLogs_20131007
GO
