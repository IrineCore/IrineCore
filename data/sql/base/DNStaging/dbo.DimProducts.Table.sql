USE [dnstaging]
GO
/****** Object:  Table [dbo].[DimProducts]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DimProducts](
	[ProductID] [int] NOT NULL,
	[ProductName] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[Price] [int] NULL,
	[Quantity] [int] NULL,
	[Lifespan] [int] NULL,
 CONSTRAINT [PK_DimProducts] PRIMARY KEY CLUSTERED 
(
	[ProductID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
