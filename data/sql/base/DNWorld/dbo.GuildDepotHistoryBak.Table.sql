USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildDepotHistoryBak]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildDepotHistoryBak](
	[GuildID] [int] NOT NULL,
	[GuildDepotHistoryID] [bigint] NOT NULL,
	[CharacterID] [int] NOT NULL,
	[CharacterName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[ItemCoinFlag] [bit] NOT NULL,
	[InOutFlag] [bit] NOT NULL,
	[ItemID] [int] NULL,
	[ItemLevel] [tinyint] NULL,
	[ItemCount] [smallint] NULL,
	[GuildCoin] [int] NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[GuildCoinAfter] [bigint] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_GuildDepotHistoryBak_GuildID_GuildDepotHistoryID]    Script Date: 11/14/2026 8:54:15 AM ******/
CREATE CLUSTERED INDEX [IX_NC_GuildDepotHistoryBak_GuildID_GuildDepotHistoryID] ON [dbo].[GuildDepotHistoryBak]
(
	[GuildID] ASC,
	[GuildDepotHistoryID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
