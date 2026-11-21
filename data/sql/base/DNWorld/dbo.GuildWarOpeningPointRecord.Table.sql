USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildWarOpeningPointRecord]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildWarOpeningPointRecord](
	[SequenceNo] [int] IDENTITY(1,1) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[MissionID] [int] NOT NULL,
	[OpeningPoint] [int] NOT NULL,
	[GuildID] [int] NOT NULL,
	[TeamColorCode] [tinyint] NOT NULL,
	[RegisterDate] [datetime] NOT NULL,
 CONSTRAINT [PK_GuildWarOpeningPointRecord] PRIMARY KEY CLUSTERED 
(
	[SequenceNo] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_GuildWarOpeningPointRecord_GuildID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_GuildWarOpeningPointRecord_GuildID] ON [dbo].[GuildWarOpeningPointRecord]
(
	[GuildID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
