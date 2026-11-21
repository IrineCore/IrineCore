USE [dnworld]
GO
/****** Object:  Table [dbo].[FairyLevelLogs0]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FairyLevelLogs0](
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
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_FailryLevelLogs0_LogDate]    Script Date: 11/14/2026 8:54:15 AM ******/
CREATE CLUSTERED INDEX [IX_NC_FailryLevelLogs0_LogDate] ON [dbo].[FairyLevelLogs0]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
