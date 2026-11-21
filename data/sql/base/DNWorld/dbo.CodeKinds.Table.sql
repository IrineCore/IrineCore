USE [dnworld]
GO
/****** Object:  Table [dbo].[CodeKinds]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CodeKinds](
	[codeColumnName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[description] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[creator] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[createdDate] [datetime] NOT NULL,
 CONSTRAINT [PK_CodeKinds] PRIMARY KEY CLUSTERED 
(
	[codeColumnName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
