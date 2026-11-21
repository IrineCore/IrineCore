USE [dnworld]
GO
/****** Object:  Table [dbo].[ChallengeDarkLairRewardCharacters]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ChallengeDarkLairRewardCharacters](
	[BasisDate] [date] NULL,
	[Ranking] [tinyint] NULL,
	[PartyMemberCount] [tinyint] NULL,
	[CharacterID] [bigint] NULL,
	[RewardFlag] [bit] NULL
) ON [DataFG1]
GO
