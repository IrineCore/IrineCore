USE [dnstaging]
GO
/****** Object:  Table [dbo].[CharacterActionLogs_20130918]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CharacterActionLogs_20130918](
	[CharacterID] [bigint] NOT NULL,
	[LogDate] [datetime2](0) NULL,
	[LogFDate] [datetime2](0) NULL,
	[CharacterActionLogCode] [tinyint] NOT NULL,
	[Amount1] [bigint] NULL,
	[Amount2] [bigint] NULL,
	[Amount3] [bigint] NULL,
	[MapID] [int] NULL,
	[IP] [int] NULL,
	[LogFKey1] [bigint] NULL,
	[LogFKey2] [bigint] NULL,
	[LogFKey3] [bigint] NULL,
	[LogMessage1] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_CharacterActionLogs_20130918_CharacterID_LogDate]    Script Date: 11/14/2026 9:00:07 AM ******/
CREATE CLUSTERED INDEX [IX_NC_CharacterActionLogs_20130918_CharacterID_LogDate] ON [dbo].[CharacterActionLogs_20130918]
(
	[CharacterID] ASC,
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[CharacterActionLogs_20130918]  WITH CHECK ADD  CONSTRAINT [CK_CharacterActionLogs_20130918_LogDate] CHECK  (([LogDate]>='2013-09-18' AND [LogDate]<'2013-09-19'))
GO
ALTER TABLE [dbo].[CharacterActionLogs_20130918] CHECK CONSTRAINT [CK_CharacterActionLogs_20130918_LogDate]
GO
