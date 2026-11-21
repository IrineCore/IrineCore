USE [dnworld]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ImportedAuctionEnableItems](
	[ItemID] [int] NOT NULL,
	[ItemCategoryID] [tinyint] NULL,
	[ItemName] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[MaxCount] [int] NOT NULL,
 CONSTRAINT [PK_ImportedAuctionEnableItems] PRIMARY KEY CLUSTERED
(
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
ALTER TABLE [dbo].[ImportedAuctionEnableItems] ADD CONSTRAINT [DF_ImportedAuctionEnableItems_MaxCount] DEFAULT ((0)) FOR [MaxCount]
GO
