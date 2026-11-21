USE [dnworld]
GO
/****** Object:  Table [dbo].[DNGirlRankingResults]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DNGirlRankingResults](
	[DNGirlSeasonID] [int] NOT NULL,
	[DNGirlCharacterID] [bigint] NOT NULL,
	[Ranking] [int] NOT NULL,
	[FanCount] [int] NOT NULL,
	[TotalIntimacy] [int] NOT NULL,
 CONSTRAINT [PK_DNGirlRankingResults] PRIMARY KEY CLUSTERED 
(
	[DNGirlSeasonID] ASC,
	[DNGirlCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[DNGirlRankingResults] ADD  CONSTRAINT [DF_DNGirlRankingResults_TotalIntimacy]  DEFAULT ((0)) FOR [TotalIntimacy]
GO
