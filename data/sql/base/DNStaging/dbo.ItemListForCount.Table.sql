USE [dnstaging]
GO
/****** Object:  Table [dbo].[ItemListForCount]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemListForCount](
	[ItemID] [int] NOT NULL,
	[ItemOption1Start] [tinyint] NULL,
	[ItemOption1End] [tinyint] NULL,
	[ItemOption2Start] [tinyint] NULL,
	[ItemOption2End] [tinyint] NULL,
	[ItemOption3Start] [tinyint] NULL,
	[ItemOption3End] [tinyint] NULL,
	[ItemCategoryID] [smallint] NULL,
 CONSTRAINT [PK_ItemListForCount] PRIMARY KEY CLUSTERED 
(
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, FILLFACTOR = 100) ON [PRIMARY]
) ON [PRIMARY]
GO
