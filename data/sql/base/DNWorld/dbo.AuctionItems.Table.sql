USE [dnworld]
GO
/****** Object:  Table [dbo].[AuctionItems]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AuctionItems](
	[ItemID] [int] NOT NULL,
	[ItemName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[LevelLimit] [tinyint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[ItemRankCode] [tinyint] NOT NULL,
	[CashItemFlag] [bit] NOT NULL,
	[ItemCategoryID] [int] NOT NULL,
 CONSTRAINT [PK_AuctionItems] PRIMARY KEY CLUSTERED 
(
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
