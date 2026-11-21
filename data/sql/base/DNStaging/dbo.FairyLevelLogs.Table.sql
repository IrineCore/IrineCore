USE [dnstaging]
GO
/****** Object:  Table [dbo].[FairyLevelLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FairyLevelLogs](
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
