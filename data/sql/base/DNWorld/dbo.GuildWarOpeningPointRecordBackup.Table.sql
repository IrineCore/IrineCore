USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildWarOpeningPointRecordBackup]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildWarOpeningPointRecordBackup](
	[SequenceNo] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[MissionID] [int] NOT NULL,
	[OpeningPoint] [int] NOT NULL,
	[GuildID] [int] NOT NULL,
	[TeamColorCode] [tinyint] NOT NULL,
	[RegisterDate] [datetime] NOT NULL,
	[BackupDate] [datetime] NOT NULL
) ON [DataFG1]
GO
