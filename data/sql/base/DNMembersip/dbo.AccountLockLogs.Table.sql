USE [dnmembership]
GO
/****** Object:  Table [dbo].[AccountLockLogs]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[AccountLockLogs]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[AccountLockLogs](
	[LogDate] [datetime2](0) NOT NULL,
	[AccountID] [int] NOT NULL,
	[LockFlag] [bit] NOT NULL
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NC_AccountLockLogs_AccountID]    Script Date: 11/14/2026 8:51:03 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[AccountLockLogs]') AND name = N'IX_NC_AccountLockLogs_AccountID')
CREATE CLUSTERED INDEX [IX_NC_AccountLockLogs_AccountID] ON [dbo].[AccountLockLogs]
(
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_AccountLockLogs_LogDate]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[AccountLockLogs]') AND name = N'IX_NN_AccountLockLogs_LogDate')
CREATE NONCLUSTERED INDEX [IX_NN_AccountLockLogs_LogDate] ON [dbo].[AccountLockLogs]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
