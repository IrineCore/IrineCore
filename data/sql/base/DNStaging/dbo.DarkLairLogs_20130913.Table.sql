USE [dnstaging]
GO
/****** Object:  Table [dbo].[DarkLairLogs_20130913]    Script Date: 11/14/2026 9:00:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DarkLairLogs_20130913](
	[LogDate] [datetime] NOT NULL,
	[WorldID] [int] NOT NULL,
	[MapID] [int] NOT NULL,
	[PartySize] [tinyint] NOT NULL,
	[PartyName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[PlayRound] [smallint] NOT NULL,
	[PlayTime] [int] NOT NULL,
	[CharacterID1] [bigint] NOT NULL,
	[CharacterLevel1] [tinyint] NOT NULL,
	[JobCode1] [tinyint] NOT NULL,
	[CharacterID2] [bigint] NULL,
	[CharacterLevel2] [tinyint] NULL,
	[JobCode2] [tinyint] NULL,
	[CharacterID3] [bigint] NULL,
	[CharacterLevel3] [tinyint] NULL,
	[JobCode3] [tinyint] NULL,
	[CharacterID4] [bigint] NULL,
	[CharacterLevel4] [tinyint] NULL,
	[JobCode4] [tinyint] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_DarkLairLogs_20130913_LogDate]    Script Date: 11/14/2026 9:00:06 AM ******/
CREATE CLUSTERED INDEX [IX_NC_DarkLairLogs_20130913_LogDate] ON [dbo].[DarkLairLogs_20130913]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[DarkLairLogs_20130913]  WITH CHECK ADD  CONSTRAINT [CK_DarkLairLogs_20130913_LogDate] CHECK  (([LogDate]>='2013-09-13' AND [LogDate]<'2013-09-14'))
GO
ALTER TABLE [dbo].[DarkLairLogs_20130913] CHECK CONSTRAINT [CK_DarkLairLogs_20130913_LogDate]
GO
