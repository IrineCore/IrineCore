USE [dngsm]
GO
/****** Object:  User [SPExecutor]    Script Date: 11/14/2026 8:47:02 AM ******/
CREATE USER [SPExecutor] WITHOUT LOGIN WITH DEFAULT_SCHEMA=[dbo]
GO
ALTER ROLE [db_owner] ADD MEMBER [SPExecutor]
GO
