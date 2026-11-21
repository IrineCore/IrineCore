USE [dngsm]
GO
/****** Object:  Table [dbo].[AuctionEnableItemsFromDOLIS]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AuctionEnableItemsFromDOLIS](
	[ItemID] [int] NOT NULL,
	[ActivateFlag] [bit] NOT NULL,
	[ItemCategoryID] [tinyint] NULL,
	[ItemName] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
 CONSTRAINT [PK_AuctionEnableItemsFromDOLIS] PRIMARY KEY CLUSTERED 
(
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
