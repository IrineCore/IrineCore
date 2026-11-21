USE [dnworld]
GO
/****** Object:  Table [dbo].[RewardAlteiaRanker]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[RewardAlteiaRanker](
	[RewardDate] [smalldatetime] NULL,
	[RewardCode] [tinyint] NULL,
	[Ranking] [int] NULL,
	[CharacterID] [bigint] NULL,
	[GuildID] [int] NULL
) ON [DataFG1]
GO
