USE [dnmembership]
GO
/****** Object:  User [web]    Script Date: 11/14/2026 8:51:03 AM ******/
IF NOT EXISTS (SELECT * FROM sys.database_principals WHERE name = N'web')
CREATE USER [web] WITHOUT LOGIN WITH DEFAULT_SCHEMA=[dbo]
GO
