USE [dnmembership]
GO
/****** Object:  Table [dbo].[AccountLogoutLogs2]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[AccountLogoutLogs2]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[AccountLogoutLogs2](
	[LogoutDate] [datetime2](0) NOT NULL,
	[AccountID] [int] NOT NULL,
	[LoginDate] [datetime2](0) NOT NULL,
	[LoginIP] [int] NOT NULL,
	[SessionID] [int] NOT NULL,
	[MachineID] [binary](16) NULL
) ON [PRIMARY]
END
GO
SET ANSI_PADDING OFF
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NC_AccountLogoutLogs2_LogoutDate]    Script Date: 11/14/2026 8:51:03 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[AccountLogoutLogs2]') AND name = N'IX_NC_AccountLogoutLogs2_LogoutDate')
CREATE CLUSTERED INDEX [IX_NC_AccountLogoutLogs2_LogoutDate] ON [dbo].[AccountLogoutLogs2]
(
	[LogoutDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_AccountLogoutLogs2_LoginDate_AccountID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[AccountLogoutLogs2]') AND name = N'IX_UN_AccountLogoutLogs2_LoginDate_AccountID')
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_AccountLogoutLogs2_LoginDate_AccountID] ON [dbo].[AccountLogoutLogs2]
(
	[LoginDate] ASC,
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
