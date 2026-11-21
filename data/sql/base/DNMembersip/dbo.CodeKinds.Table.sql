USE [dnmembership]
GO
/****** Object:  Table [dbo].[CodeKinds]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CodeKinds]') AND type in (N'U'))
BEGIN
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
END
GO
