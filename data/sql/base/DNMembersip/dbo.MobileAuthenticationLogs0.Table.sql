USE [dnmembership]
GO
/****** Object:  Table [dbo].[MobileAuthenticationLogs0]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[MobileAuthenticationLogs0]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[MobileAuthenticationLogs0](
	[LogDate] [datetime2](0) NOT NULL,
	[AccountID] [int] NOT NULL,
	[AuthenticationLogCode] [tinyint] NOT NULL,
	[AccountAuthenticationKey] [int] NULL
) ON [DataFG1]
END
GO
