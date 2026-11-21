USE [dnmembership]
GO
/****** Object:  Table [dbo].[SecondAuthLogs]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[SecondAuthLogs]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[SecondAuthLogs](
	[LogDate] [datetime2](0) NOT NULL,
	[AccountID] [int] NOT NULL,
	[SecondAuthCode] [tinyint] NOT NULL
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NC_SecondAuthLogs_LogDate]    Script Date: 11/14/2026 8:51:04 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[SecondAuthLogs]') AND name = N'IX_NC_SecondAuthLogs_LogDate')
CREATE CLUSTERED INDEX [IX_NC_SecondAuthLogs_LogDate] ON [dbo].[SecondAuthLogs]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_SecondAuthLogs_AccountID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[SecondAuthLogs]') AND name = N'IX_NN_SecondAuthLogs_AccountID')
CREATE NONCLUSTERED INDEX [IX_NN_SecondAuthLogs_AccountID] ON [dbo].[SecondAuthLogs]
(
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_SecondAuthLogs_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[SecondAuthLogs]'))
ALTER TABLE [dbo].[SecondAuthLogs]  WITH CHECK ADD  CONSTRAINT [FK_Accounts_SecondAuthLogs_AccountID] FOREIGN KEY([AccountID])
REFERENCES [dbo].[Accounts] ([AccountID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_SecondAuthLogs_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[SecondAuthLogs]'))
ALTER TABLE [dbo].[SecondAuthLogs] CHECK CONSTRAINT [FK_Accounts_SecondAuthLogs_AccountID]
GO
