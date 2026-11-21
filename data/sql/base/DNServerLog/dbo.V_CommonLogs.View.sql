USE [dnserverlog]
GO
/****** Object:  View [dbo].[V_CommonLogs]    Script Date: 11/14/2026 8:57:59 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

CREATE VIEW [dbo].[V_CommonLogs]
AS
SELECT LogDate, LogType, ServerType, WorldID, AccountID, CharacterID, SessionID, LogText FROM dbo.CommonLogs20140921 UNION ALL
SELECT LogDate, LogType, ServerType, WorldID, AccountID, CharacterID, SessionID, LogText FROM dbo.CommonLogs20140922 UNION ALL
SELECT LogDate, LogType, ServerType, WorldID, AccountID, CharacterID, SessionID, LogText FROM dbo.CommonLogs20140924 UNION ALL
SELECT LogDate, LogType, ServerType, WorldID, AccountID, CharacterID, SessionID, LogText FROM dbo.CommonLogs20140925 UNION ALL
SELECT LogDate, LogType, ServerType, WorldID, AccountID, CharacterID, SessionID, LogText FROM dbo.CommonLogs20140926 UNION ALL
SELECT LogDate, LogType, ServerType, WorldID, AccountID, CharacterID, SessionID, LogText FROM dbo.CommonLogs20141003 
GO
