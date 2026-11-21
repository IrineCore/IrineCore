USE [dnstaging]
GO
/****** Object:  Table [dbo].[FairyLevelLogs_20130917]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FairyLevelLogs_20130917](
	[LogDate] [datetime2](7) NOT NULL,
	[FairyItemSerial] [bigint] NOT NULL,
	[FairyLevelBefore] [tinyint] NOT NULL,
	[FairyLevelCurrent] [tinyint] NOT NULL,
	[ChangedStatus1] [int] NULL,
	[ChangedStatus2] [int] NULL,
	[ChangedStatus3] [int] NULL,
	[ChangedStatus4] [int] NULL,
	[ChangedStatus5] [int] NULL,
	[ChangedStatus6] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_FairyLevelLogs_20130917_LogDate]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_FairyLevelLogs_20130917_LogDate] ON [dbo].[FairyLevelLogs_20130917]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[FairyLevelLogs_20130917]  WITH CHECK ADD  CONSTRAINT [CK_FairyLevelLogs_20130917_LogDate] CHECK  (([LogDate]>='2013-09-17' AND [LogDate]<'2013-09-18'))
GO
ALTER TABLE [dbo].[FairyLevelLogs_20130917] CHECK CONSTRAINT [CK_FairyLevelLogs_20130917_LogDate]
GO
