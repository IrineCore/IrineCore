USE [dnworld]
GO
/****** Object:  Table [dbo].[AutoTrade]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AutoTrade](
	[ItemID] [int] NOT NULL,
	[MaxUnitPrice] [int] NOT NULL,
	[RemainBudget] [bigint] NOT NULL,
	[EnableFlag] [bit] NOT NULL,
 CONSTRAINT [PK_AutoTrade] PRIMARY KEY CLUSTERED 
(
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
