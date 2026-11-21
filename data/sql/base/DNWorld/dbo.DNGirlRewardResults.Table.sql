USE [dnworld]
GO
/****** Object:  Table [dbo].[DNGirlRewardResults]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DNGirlRewardResults](
	[DNGirlCharacterID] [bigint] NOT NULL,
	[RewardTypeCode] [tinyint] NOT NULL,
	[DNGirlRewardFlag] [bit] NOT NULL,
	[FanRewardFlag] [bit] NOT NULL,
 CONSTRAINT [PK_DNGirlRewardResults] PRIMARY KEY CLUSTERED 
(
	[DNGirlCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
