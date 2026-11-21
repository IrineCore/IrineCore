USE [dnserverlog]
GO
/****** Object:  View [dbo].[V_ServerPingLogs]    Script Date: 11/14/2026 8:57:59 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

CREATE VIEW [dbo].[V_ServerPingLogs]
AS
SELECT * FROM dbo.ServerPingLogs20140921 UNION ALL
SELECT * FROM dbo.ServerPingLogs20140922 UNION ALL
SELECT * FROM dbo.ServerPingLogs20140924 UNION ALL
SELECT * FROM dbo.ServerPingLogs20140926 UNION ALL
SELECT * FROM dbo.ServerPingLogs20141003 
GO
