USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPLadderRanking]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPLadderRanking](
	[CharacterID] [bigint] NOT NULL,
	[CharacterName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[CharacterClassCode] [tinyint] NOT NULL,
	[CharacterSubClassCode] [tinyint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[GuildID] [int] NULL,
	[GuildName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NULL,
	[PVPLadderGradePoint] [int] NOT NULL,
	[Win] [int] NOT NULL,
	[Lose] [int] NOT NULL,
	[Draw] [int] NOT NULL,
	[WinRatio] [int] NOT NULL,
	[PVPClassRank] [int] NOT NULL,
	[PVPClassRatio] [float] NOT NULL,
	[TotalRank] [bigint] NOT NULL,
	[CharacterClassRank] [bigint] NOT NULL,
	[CharacterSubClassRank] [bigint] NOT NULL,
	[ChangedTotalRank] [bigint] NULL,
	[ChangedCharacterClassRank] [bigint] NULL,
	[ChangedCharacterSubClassRank] [bigint] NULL,
	[PVPLadderCode] [tinyint] NOT NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_PVPLadderRanking_TotalRank]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_PVPLadderRanking_TotalRank] ON [dbo].[PVPLadderRanking]
(
	[TotalRank] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_PVPLadderRanking]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[PVPLadderRanking] ADD  CONSTRAINT [PK_PVPLadderRanking] PRIMARY KEY NONCLUSTERED 
(
	[CharacterID] ASC,
	[PVPLadderCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_PVPLadderRanking_CharacterClassRank]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PVPLadderRanking_CharacterClassRank] ON [dbo].[PVPLadderRanking]
(
	[CharacterClassRank] ASC,
	[CharacterClassCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NN_PVPLadderRanking_CharacterName]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PVPLadderRanking_CharacterName] ON [dbo].[PVPLadderRanking]
(
	[CharacterName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_PVPLadderRanking_CharacterSubClassRank]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PVPLadderRanking_CharacterSubClassRank] ON [dbo].[PVPLadderRanking]
(
	[CharacterSubClassRank] ASC,
	[CharacterSubClassCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NN_PVPLadderRanking_GuildName]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PVPLadderRanking_GuildName] ON [dbo].[PVPLadderRanking]
(
	[GuildName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
