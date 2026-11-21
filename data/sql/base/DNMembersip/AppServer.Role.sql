USE [dnmembership]
GO
/****** Object:  DatabaseRole [AppServer]    Script Date: 11/14/2026 8:51:03 AM ******/
IF NOT EXISTS (SELECT * FROM sys.database_principals WHERE name = N'AppServer' AND type = 'R')
CREATE ROLE [AppServer]
GO
