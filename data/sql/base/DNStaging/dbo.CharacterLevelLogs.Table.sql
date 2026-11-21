USE [dnstaging]
GO
/****** Object:  Table [dbo].[CharacterLevelLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CharacterLevelLogs](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[CharacterLevelChangeCode] [tinyint] NOT NULL,
	[CurrentCharacterLevel] [tinyint] NOT NULL,
	[ChangedCharacterLevel] [smallint] NOT NULL,
	[ChannelID] [int] NULL,
	[MapID] [int] NULL
) ON [PRIMARY]
GO
