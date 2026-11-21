USE [dnstaging]
GO
/****** Object:  Table [dbo].[CharacterLevelLogs_20131004]    Script Date: 11/14/2026 9:00:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CharacterLevelLogs_20131004](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[CharacterLevelChangeCode] [tinyint] NOT NULL,
	[CurrentCharacterLevel] [tinyint] NOT NULL,
	[ChangedCharacterLevel] [smallint] NOT NULL,
	[ChannelID] [int] NULL,
	[MapID] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_CharacterLevelLogs_20131004_LogDate]    Script Date: 11/14/2026 9:00:06 AM ******/
CREATE CLUSTERED INDEX [IX_NC_CharacterLevelLogs_20131004_LogDate] ON [dbo].[CharacterLevelLogs_20131004]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_CharacterLevelLogs_20131004_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_CharacterLevelLogs_20131004_CharacterID] ON [dbo].[CharacterLevelLogs_20131004]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[CharacterLevelLogs_20131004]  WITH CHECK ADD  CONSTRAINT [CK_CharacterLevelLogs_20131004_LogDate] CHECK  (([LogDate]>='2013-10-04' AND [LogDate]<'2013-10-05'))
GO
ALTER TABLE [dbo].[CharacterLevelLogs_20131004] CHECK CONSTRAINT [CK_CharacterLevelLogs_20131004_LogDate]
GO
